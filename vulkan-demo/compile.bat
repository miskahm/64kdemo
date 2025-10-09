@echo off
echo Compiling Vulkan Demo...
C:\msys64\mingw64\bin\gcc.exe -std=c99 -Isrc -IC:/VulkanSDK/1.4.321.1/Include -IC:/msys64/mingw64/include -g -O0 -o build/Vulkan64KDemo_debug.exe src/main.c src/vulkan_setup.c src/shader_loader.c src/shadertoy_compat.c src/audio_synthesis.c src/sync_system.c -LC:/VulkanSDK/1.4.321.1/Lib -LC:/msys64/mingw64/lib -lvulkan-1 -lglfw3 -lwinmm -lgdi32 -luser32 -lkernel32 -static-libgcc 1>build/compile.log 2>&1
echo.
echo Exit code: %ERRORLEVEL%
echo.
echo === Compilation output ===
type build\compile.log
echo === End output ===
if exist build/Vulkan64KDemo_debug.exe (
    echo SUCCESS - Executable created
) else (
    echo FAILED - No executable created
)
