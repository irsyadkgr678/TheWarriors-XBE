# Toolchain portabel (tanpa admin) untuk build The Warriors - XBE Recomp.
#
# Pakai di PowerShell:
#   . .\setup-env.ps1        (tanda titik di depan = dijalankan di sesi ini)
# lalu:
#   cmake --preset release
#
# Untuk build normal cukup dobel-klik build-here.bat (sudah mengatur PATH sendiri).

$projectRoot = $PSScriptRoot
$toolchain   = Join-Path (Split-Path $projectRoot) "toolchain"

if (-not (Test-Path (Join-Path $toolchain "cmake\bin\cmake.exe"))) {
    Write-Error "Toolchain tidak ditemukan di $toolchain. Jalankan _setup\install_toolchain.py dulu."
    return
}

$env:PATH = "$toolchain\git\cmd;$toolchain\cmake\bin;$toolchain\ninja;$env:PATH"

# Di sandbox pengembangan, TLS bawaan Windows (schannel) diblokir sehingga git gagal
# dengan SEC_E_NO_CREDENTIALS. Backend OpenSSL milik git berfungsi normal, dan tidak
# mengganggu di Windows biasa.
$env:GIT_CONFIG_GLOBAL = Join-Path $toolchain "gitconfig"

# Kalau Visual Studio sudah terpasang, muat environment MSVC + Windows SDK-nya.
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vswhere) {
    $vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ($vsPath) {
        $devCmd = Join-Path $vsPath "Common7\Tools\VsDevCmd.bat"
        if (Test-Path $devCmd) {
            cmd /c "`"$devCmd`" -arch=amd64 -host_arch=amd64 >nul && set" | ForEach-Object {
                if ($_ -match '^([^=]+)=(.*)$') { Set-Item -Path "env:$($matches[1])" -Value $matches[2] }
            }
            Write-Host "MSVC + Windows SDK dimuat dari $vsPath"
        }
    }
}

Write-Host ""
Write-Host "git    : $(& git --version 2>$null)"
Write-Host "cmake  : $((& cmake --version 2>$null) | Select-Object -First 1)"
Write-Host "ninja  : $(& ninja --version 2>$null)"
Write-Host "clang++: $(if (Get-Command clang++ -ErrorAction SilentlyContinue) { (& clang++ --version | Select-Object -First 1) } else { 'BELUM ADA (butuh Visual Studio + C++ Clang tools for Windows)' })"
