@echo off
setlocal
cd /d "%~dp0"
if not defined NOB_SRC set NOB_SRC=nob.c
if not exist nob.exe (
    gcc -o nob.exe %NOB_SRC%
    if errorlevel 1 exit /b 1
)
nob.exe %*
