@echo off
echo ============================================
echo Building Vulkan 64K Demo
echo ============================================
echo.

if not exist build mkdir build

echo [1/4] Copying dependencies...
copy /Y "C:\msys64\mingw64\bin\glfw3.dll" build\ >nul
if errorlevel 1 (
    echo ERROR: Failed to copy glfw3.dll
    exit /b 1
)

echo [2/4] Compiling shaders...
C:\VulkanSDK\1.4.321.1\Bin\glslangValidator.exe -V shaders\shader.vert -o build\shader.vert.spv
if errorlevel 1 (
    echo ERROR: Failed to compile vertex shader
    exit /b 1
)
C:\VulkanSDK\1.4.321.1\Bin\glslangValidator.exe -V shaders\shader.frag -o build\shader.frag.spv
if errorlevel 1 (
    echo ERROR: Failed to compile fragment shader
    exit /b 1
)

echo [3/4] Compiling demo (debug build)...
C:\msys64\usr\bin\bash.exe -c "export PATH=/mingw64/bin:$PATH && cd /e/projects/64kdemo/vulkan-demo && gcc -std=c99 -Isrc -I/c/VulkanSDK/1.4.321.1/Include -I/mingw64/include -g -O0 -o build/Vulkan64KDemo_debug.exe src/main.c src/vulkan_setup.c src/shader_loader.c src/shadertoy_compat.c src/audio_synthesis.c src/sync_system.c -L/c/VulkanSDK/1.4.321.1/Lib -L/mingw64/lib -lvulkan-1 -lglfw3 -lwinmm -lgdi32 -luser32 -lkernel32 -static-libgcc"
if errorlevel 1 (
    echo ERROR: Compilation failed
    exit /b 1
)

echo [4/4] Compiling demo (release build)...
C:\msys64\usr\bin\bash.exe -c "export PATH=/mingw64/bin:$PATH && cd /e/projects/64kdemo/vulkan-demo && gcc -std=c99 -Isrc -I/c/VulkanSDK/1.4.321.1/Include -I/mingw64/include -Os -s -ffast-math -ffunction-sections -fdata-sections -o build/Vulkan64KDemo.exe src/main.c src/vulkan_setup.c src/shader_loader.c src/shadertoy_compat.c src/audio_synthesis.c src/sync_system.c -L/c/VulkanSDK/1.4.321.1/Lib -L/mingw64/lib -lvulkan-1 -lglfw3 -lwinmm -lgdi32 -luser32 -lkernel32 -static-libgcc -Wl,--gc-sections"
if errorlevel 1 (
    echo ERROR: Release compilation failed
    exit /b 1
)

echo.
echo ============================================
echo Build complete!
echo ============================================
echo Debug build: build\Vulkan64KDemo_debug.exe
echo Release build: build\Vulkan64KDemo.exe
echo.
dir build\*.exe | find "Vulkan64KDemo"
echo.
