#include "shader_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

static char* readFile(const char* filename, size_t* fileSize) {
    FILE* file = NULL;
    char altPath[512];
    
    file = fopen(filename, "rb");
    
    if (!file) {
        snprintf(altPath, sizeof(altPath), "build/%s", filename);
        file = fopen(altPath, "rb");
    }
    
    if (!file) {
        snprintf(altPath, sizeof(altPath), "../build/%s", filename);
        file = fopen(altPath, "rb");
    }
    
    if (!file) {
        fprintf(stderr, "FATAL: Failed to open file: %s\n", filename);
        fprintf(stderr, "  Tried paths: %s, build/%s, ../build/%s\n", filename, filename, filename);
        fprintf(stderr, "  System error: %s\n", strerror(errno));
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* buffer = malloc(*fileSize);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate memory for file: %s\n", filename);
        fclose(file);
        return NULL;
    }
    
    fread(buffer, 1, *fileSize, file);
    fclose(file);
    
    return buffer;
}

VkShaderModule createShaderModule(VkDevice device, const char* filename) {
    size_t codeSize;
    char* code = readFile(filename, &codeSize);
    if (!code) {
        return VK_NULL_HANDLE;
    }
    
    VkShaderModuleCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = codeSize;
    createInfo.pCode = (const uint32_t*)code;
    
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create shader module: %s\n", filename);
        free(code);
        return VK_NULL_HANDLE;
    }
    
    free(code);
    return shaderModule;
}

bool compileShaderToSPIRV(const char* glslFile, const char* spirvFile) {
    char command[512];
    snprintf(command, sizeof(command), "glslangValidator -V %s -o %s", glslFile, spirvFile);
    
    int result = system(command);
    if (result != 0) {
        fprintf(stderr, "Failed to compile shader: %s\n", glslFile);
        return false;
    }
    
    return true;
}