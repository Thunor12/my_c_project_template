# Generate output_project/ from config/catalog.yaml.
# Usage:
#   .\scripts\render-output-project.ps1
#   $env:DEPS = "sqlite"; .\scripts\render-output-project.ps1
#   $env:DEPS = "sqlite,teapot"; .\scripts\render-output-project.ps1
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..")

$deps = if ($env:DEPS) { $env:DEPS } else { "" }
python scripts/render_project.py --deps $deps --output output_project
