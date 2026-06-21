# Generate output_project/ with every dep from config/catalog.yaml.
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..")

$deps = python -c "import yaml; print(','.join(yaml.safe_load(open('config/catalog.yaml'))['deps']))"
python scripts/render_project.py --deps $deps --output output_project
