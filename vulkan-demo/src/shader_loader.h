#ifndef SHADER_LOADER_H
#define SHADER_LOADER_H

#include <vulkan/vulkan.h>
#include <stdbool.h>

VkShaderModule createShaderModule(VkDevice device, const char* filename);
bool compileShaderToSPIRV(const char* glslFile, const char* spirvFile);

#endif