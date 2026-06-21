# Enable teapot: activate config/features/teapot and fetch the submodule.
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..")

if (-not (Test-Path "config/features/teapot/teapot.h")) {
    Write-Error "config/features/teapot/ not found"
}

New-Item -ItemType Directory -Force -Path "config/enabled" | Out-Null
Copy-Item "config/features/teapot/teapot.h" "config/enabled/teapot.h" -Force
Copy-Item "config/features/teapot/teapot.build.h" "config/enabled/teapot.build.h" -Force
Write-Host "activated config/enabled/teapot.h + teapot.build.h"

$gitmodules = Get-Content ".gitmodules" -Raw -ErrorAction SilentlyContinue
if ($gitmodules -notmatch "third_party/teapot") {
    git submodule add https://github.com/Thunor12/teapot.git third_party/teapot
}
git submodule update --init --depth 1 third_party/teapot

gcc -o nob.exe nob.c
Write-Host "teapot enabled — run: ./nob.exe examples"
