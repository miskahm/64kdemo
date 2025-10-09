@echo off
echo Building 64K Demo with Crinkler compression...
echo.

REM Check if crinkler.exe exists
if not exist "crinkler.exe" (
    echo ERROR: crinkler.exe not found in current directory!
    echo Please download Crinkler from https://github.com/runestubbe/Crinkler
    pause
    exit /b 1
)

REM Clean previous build
echo Cleaning previous build...
if exist build rmdir /s /q build
if exist Vulkan64KDemo_64k.exe del Vulkan64KDemo_64k.exe

REM Build with Makefile
echo Building optimized binary...
make -f Makefile.win all
if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo Binary size before compression:
dir build\Vulkan64KDemo.exe | find "Vulkan64KDemo"

REM Compress with Crinkler
echo.
echo Compressing with Crinkler...
crinkler.exe /COMPMODE:MAX /OUT:Vulkan64KDemo_64k.exe /UNSAFEIMPORT /TINYHEADER /HASH:EMPHASH build\Vulkan64KDemo.exe
if %ERRORLEVEL% neq 0 (
    echo ERROR: Crinkler compression failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo FINAL COMPRESSED SIZE:
dir Vulkan64KDemo_64k.exe | find "Vulkan64KDemo_64k"
echo ========================================

REM Check if under 64KB
for %%F in (Vulkan64KDemo_64k.exe) do set size=%%~zF
set /a sizeKB=%size%/1024

echo Size: %sizeKB% KB

if %sizeKB% leq 64 (
    echo SUCCESS: Under 64KB limit!
) else (
    echo WARNING: Over 64KB limit (%sizeKB% KB)
)

echo.
echo Build complete! Output: Vulkan64KDemo_64k.exe
pause