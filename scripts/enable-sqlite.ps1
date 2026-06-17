# Enable SQLite: activate config/features/sqlite and fetch the amalgamation.
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..")

if (-not (Test-Path "config/features/sqlite/sqlite.h")) {
    Write-Error "config/features/sqlite/ not found"
}

New-Item -ItemType Directory -Force -Path "config/enabled" | Out-Null
Copy-Item "config/features/sqlite/sqlite.h" "config/enabled/sqlite.h" -Force
Copy-Item "config/features/sqlite/sqlite.build.h" "config/enabled/sqlite.build.h" -Force
Write-Host "activated config/enabled/sqlite.h + sqlite.build.h"

$nobSrc = if (Test-Path "nob.template.c") { "nob.template.c" } else { "nob.c" }
gcc -o nob.exe $nobSrc
./nob.exe setup sqlite
Write-Host "sqlite enabled — run: ./nob.exe examples"
