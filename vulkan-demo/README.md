# 64K Vulkan Demoscene Intro

A 64KB demoscene intro using Vulkan and ShaderToy-style shaders written in pure C.

## Features
- Vulkan rendering with fullscreen quad
- ShaderToy-compatible uniforms (iTime, iResolution, iMouse, iFrame)
- SPIR-V shader compilation
- Size-optimized build targeting <64KB executable
- Cross-platform (Windows/Linux)

## Build Requirements
- Vulkan SDK 1.3+
- GLFW3
- CMake 3.10+
- glslangValidator (from Vulkan SDK)

## Build Instructions

```bash
# Clone and build
cd vulkan-demo
mkdir build && cd build
cmake ..
make

# Run
./Vulkan64KDemo
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
- Final compression with Crinkler (Windows) for <64KB target

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

## Next Steps
- [ ] Add audio synthesis (sointu/4klang)
- [ ] Implement Rocket sync for music-visual synchronization
- [ ] Add visual effects (raymarching, fractals, particles)
- [ ] Apply shader minification
- [ ] Final compression with Crinkler