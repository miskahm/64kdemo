cmake_minimum_required(VERSION 3.10)
project(Vulkan64KDemo C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find packages
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

# Aggressive size optimization flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -s -ffast-math -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -ffunction-sections -fdata-sections -fno-ident")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections -Wl,--strip-all -Wl,--build-id=none -static-libgcc -static-libstdc++ -Wl,-s")

# Link Time Optimization
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
endif()

# Include directories
include_directories(${Vulkan_INCLUDE_DIRS})
include_directories(src)

# Source files
set(SOURCES
    src/main.c
    src/vulkan_setup.c
    src/shader_loader.c
    src/shadertoy_compat.c
    src/audio_synthesis.c
    src/sync_system.c
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES} glfw)

# Windows audio libraries (minimal)
if(WIN32)
    target_link_libraries(${PROJECT_NAME} winmm)
endif()

# Shader compilation
find_program(GLSLANG_VALIDATOR glslangValidator)

if(GLSLANG_VALIDATOR)
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/shader.vert.spv ${CMAKE_BINARY_DIR}/shader.frag.spv
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}
        COMMAND ${GLSLANG_VALIDATOR} -V ${CMAKE_SOURCE_DIR}/shaders/shader.vert -o ${CMAKE_BINARY_DIR}/shader.vert.spv
        COMMAND ${GLSLANG_VALIDATOR} -V ${CMAKE_SOURCE_DIR}/shaders/shader.frag -o ${CMAKE_BINARY_DIR}/shader.frag.spv
        DEPENDS ${CMAKE_SOURCE_DIR}/shaders/shader.vert ${CMAKE_SOURCE_DIR}/shaders/shader.frag
        COMMENT "Compiling shaders to SPIR-V"
    )
    
    add_custom_target(shaders ALL DEPENDS ${CMAKE_BINARY_DIR}/shader.vert.spv ${CMAKE_BINARY_DIR}/shader.frag.spv)
    add_dependencies(${PROJECT_NAME} shaders)
endif()

# Find Crinkler
find_program(CRINKLER crinkler)

# Post-build: Check size
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Binary size:"
        COMMAND dir ${CMAKE_BINARY_DIR}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}
    )
    
    # Crinkler compression target
    if(CRINKLER)
        add_custom_target(crinkler
            COMMAND ${CMAKE_COMMAND} -E echo "Compressing with Crinkler..."
            COMMAND ${CRINKLER} /COMPMODE:MAX /OUT:${CMAKE_BINARY_DIR}/${PROJECT_NAME}_64k.exe /UNSAFEIMPORT /TINYHEADER /HASH:EMPHASH ${CMAKE_BINARY_DIR}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}
            COMMAND ${CMAKE_COMMAND} -E echo "Compressed binary size:"
            COMMAND dir ${CMAKE_BINARY_DIR}/${PROJECT_NAME}_64k.exe
            DEPENDS ${PROJECT_NAME}
            COMMENT "Compressing executable with Crinkler"
        )
        
        add_custom_target(release
            DEPENDS crinkler
            COMMENT "Building release with Crinkler compression"
        )
    endif()
else()
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Binary size:"
        COMMAND ls -lh ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
    )
endif()