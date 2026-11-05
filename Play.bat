@echo off
rem Menjalankan The Warriors: launcher kalau ada (grafik, controller, keyboard, folder game), else game langsung.
setlocal
set "REL=%~dp0out\build\release"
if not exist "%REL%\TheWarriors.exe" (
    echo Belum ada TheWarriors.exe. Jalankan build-here.bat dulu.
    pause
    exit /b 1
)
cd /d "%REL%"
if exist TheWarriorsLauncher.exe (
    start "" TheWarriorsLauncher.exe
) else (
    start "" TheWarriors.exe
)
