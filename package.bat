@echo off
rem Menyusun zip rilis TANPA 7-Zip. Isinya alat saja: executable, launcher, CRT, compiler,
rem dan folder assets kosong. Tidak ada TheWarriors.dll dan tidak ada data game.
setlocal
cd /d "%~dp0"
set "OUT=out\build\release"
set "STAGE=out\package"
set "NAME=TheWarriors-port-v1.0.zip"

if not exist "%OUT%\TheWarriors.exe" (
    echo Belum ada build. Jalankan build-here.bat dulu.
    exit /b 1
)
if not exist "%OUT%\TheWarriorsLauncher.exe" (
    echo TheWarriorsLauncher.exe belum ada di %OUT%.
    exit /b 1
)

echo [1/3] Menyusun folder paket
rd /s /q "%STAGE%" 2>nul
mkdir "%STAGE%\assets" 2>nul
mkdir "%STAGE%\compiler" 2>nul

copy /y "%OUT%\TheWarriors.exe" "%STAGE%" >nul
copy /y "%OUT%\TheWarriorsLauncher.exe" "%STAGE%" >nul
for %%f in (msvcp140.dll vcruntime140.dll vcruntime140_1.dll) do (
    if exist "%OUT%\%%f" copy /y "%OUT%\%%f" "%STAGE%" >nul
)

echo [2/3] Menyalin compiler tanpa work\ (residu build) dan tanpa kode game
robocopy "%OUT%\compiler" "%STAGE%\compiler" /E /XD work /NFL /NDL /NJH /NJS >nul
if errorlevel 8 (
    echo robocopy gagal menyusun folder compiler.
    exit /b 1
)
rem xbe_recomp.exe butuh CRT di folder yang sama
for %%f in (msvcp140.dll vcruntime140.dll vcruntime140_1.dll) do (
    if exist "%OUT%\%%f" copy /y "%OUT%\%%f" "%STAGE%\compiler" >nul
)

copy /y README.md "%STAGE%" >nul
copy /y THIRD_PARTY.md "%STAGE%" >nul
copy /y LICENSE "%STAGE%" >nul
rem folder assets harus tetap ada di zip (zip tidak menyimpan folder kosong):
rem file teks milik kita sendiri, bukan data game
(
    echo Taruh data game milikmu di folder ini.
    echo.
    echo Salah satu bentuk:
    echo   - satu file .iso Xbox NTSC/USA  ^(paling mudah, tidak perlu ekstrak^)
    echo   - hasil ekstrak lengkap: default.xbe + Main.wad* + Paks.wad* + System.wad0
    echo     + XBoxWad.idx + audio\ + bik\
    echo.
    echo JANGAN hanya menaruh default.xbe sendirian tanpa file WAD: game akan mati
    echo beberapa ratus milidetik setelah boot.
    echo.
    echo Data game bisa juga diletakkan di drive lain; arahkan lewat launcher,
    echo tab "Folder game".
) > "%STAGE%\assets\TARUH-GAME-DI-SINI.txt"

echo [3/3] Membuat %NAME%
del /q "%NAME%" 2>nul
powershell -NoProfile -Command "Compress-Archive -Path '%STAGE%\*' -DestinationPath '%NAME%' -Force"
if not exist "%NAME%" (
    echo Gagal membuat zip.
    exit /b 1
)

echo.
echo Selesai: %NAME%
echo   isi   : TheWarriors.exe, TheWarriorsLauncher.exe, CRT DLL, compiler\, assets\ (kosong), README, LICENSE
echo   TIDAK : TheWarriors.dll, data game apa pun
echo   pemain membangun TheWarriors.dll dari default.xbe miliknya pada start pertama.
