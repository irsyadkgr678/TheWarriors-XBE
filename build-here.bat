@echo off
rem Jalankan build.bat dengan toolchain portabel di ..\toolchain (tanpa perlu admin / PATH global).
setlocal
set "TC=%~dp0..\toolchain"
set "PATH=%TC%\git\cmd;%TC%\cmake\bin;%TC%\ninja;%PATH%"
set "GIT_CONFIG_GLOBAL=%TC%\gitconfig"
call "%~dp0build.bat" %*
exit /b %errorlevel%
