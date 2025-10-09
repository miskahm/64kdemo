# 64K Vulkan Demoscene Intro

A **34.5 KB** demoscene intro using Vulkan and ShaderToy-style shaders written in pure C.

## Features
- ✅ **Final size: 34.5 KB (under 64KB limit!)**
- Vulkan rendering with fullscreen quad
- Real-time audio synthesis (140 BPM)
- Audio-reactive visuals (bass/mid/high frequency bands)
- ShaderToy-compatible uniforms (iTime, iResolution, iMouse, iFrame, iBass, iMid, iHigh)
- SPIR-V shader compilation
- 5 unique shader scenes with 12-second transitions
- UPX compressed executable

## Build Requirements
- Vulkan SDK 1.3+
- GLFW3
- CMake 3.10+
- glslangValidator (from Vulkan SDK)

## Quick Start

**Download and run the compressed 64K intro:**
```bash
Vulkan64KDemo_64k.exe    # 34.5 KB - Ready to run!
```

## Build Instructions

```bash
# Build from source
cd vulkan-demo
build.bat                # Builds debug + release

# Run uncompressed builds
build\Vulkan64KDemo_debug.exe    # Debug build (914 KB)
build\Vulkan64KDemo.exe          # Release build (70 KB)

# Compress with UPX (optional - already included)
upx.exe --best --ultra-brute -o Vulkan64KDemo_64k.exe build\Vulkan64KDemo.exe
# Result: 34.5 KB compressed executable
```

## Project Structure
```
src/
├── main.c              # Entry point and main loop
├── vulkan_setup.c      # Vulkan initialization
├── shader_loader.c     # SPIR-V shader loading
└── shadertoy_compat.c  # ShaderToy uniform handling

shaders/
├── shader.vert         # Fullscreen quad vertex shader
└── shader.frag         # Fragment shader (ShaderToy style)
```

## Development Workflow
1. Edit shaders in `shaders/` directory
2. Build with `make` (auto-compiles shaders to SPIR-V)
3. Test with `./Vulkan64KDemo`
4. Optimize for size with release build: `cmake -DCMAKE_BUILD_TYPE=Release ..`

## Size Optimization
- Release build uses `-Os` for size optimization
- Static linking to reduce dependencies
- Strip symbols with `-s`
- **UPX compression: 71,680 → 35,328 bytes (49.29% ratio)**
- **Final size: 34.5 KB** (46% under 64KB limit) ✅

## Shader Development
Shaders use ShaderToy-compatible uniforms:
```glsl
layout(binding = 0) uniform UniformBufferObject {
    float iTime;        // Time in seconds
    vec2 iResolution;   // Viewport resolution
    vec4 iMouse;        // Mouse position
    int iFrame;         // Frame counter
} ubo;
```

## Features Completed ✅
- ✅ Audio synthesis (miniaudio-based)
- ✅ Audio-reactive visuals synchronized to music
- ✅ Visual effects (raymarching, fractals, particles, Menger cubes)
- ✅ Shader minification
- ✅ Final compression with UPX (34.5 KB)
- ✅ **64KB target achieved!**

## Build Stats
- Debug build: 935,814 bytes (914 KB)
- Release build: 71,680 bytes (70 KB)
- **Compressed: 35,328 bytes (34.5 KB)** 🎉
- Compression ratio: 49.29%
- Under 64KB limit by: 30,208 bytes (46%)