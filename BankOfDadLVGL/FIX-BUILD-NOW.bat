@echo off
REM Bank of Dad — overwrite broken local files from GitHub (no git required)
set BASE=https://raw.githubusercontent.com/sumkindafreak/bank-of-dad/cursor/bank-of-dad-v22-dcaf/BankOfDadLVGL
set DEST=%~dp0
if not "%~1"=="" set DEST=%~1

echo Fixing Bank of Dad build in: %DEST%
echo.

curl -fsSL "%BASE%/BankOfDadLVGL.ino" -o "%DEST%BankOfDadLVGL.ino" || goto fail
curl -fsSL "%BASE%/touch_lvgl.h" -o "%DEST%touch_lvgl.h" || goto fail
curl -fsSL "%BASE%/rgb_sync.h" -o "%DEST%rgb_sync.h" || goto fail
curl -fsSL "%BASE%/touch_lvgl.cpp" -o "%DEST%touch_lvgl.cpp" || goto fail
curl -fsSL "%BASE%/rgb_sync.cpp" -o "%DEST%rgb_sync.cpp" || goto fail
curl -fsSL "%BASE%/lv_conf.h" -o "%DEST%lv_conf.h" || goto fail

set LIB=%USERPROFILE%\Documents\Arduino\libraries
if not exist "%LIB%" mkdir "%LIB%"
curl -fsSL "https://raw.githubusercontent.com/sumkindafreak/bank-of-dad/cursor/bank-of-dad-v22-dcaf/arduino-libraries/lv_conf.h" -o "%LIB%\lv_conf.h" || goto fail

echo.
echo Done. Open %DEST%BankOfDadLVGL.ino and compile.
echo Check BankOfDadLVGL.ino contains: BANK_OF_DAD_BUILD 222
pause
exit /b 0

:fail
echo Download failed. Check internet connection.
pause
exit /b 1
