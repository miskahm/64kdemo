#ifndef SHADERTOY_COMPAT_H
#define SHADERTOY_COMPAT_H

#include "vulkan_setup.h"
#include "audio_synthesis.h"
#include "sync_system.h"
#include <vulkan/vulkan.h>

void updateUniforms(DemoApp* app, float currentTime, int frame, AudioEngine* audio, RocketSync* sync);
VkDescriptorSetLayoutBinding createUniformBinding();
VkWriteDescriptorSet createUniformWrite(VkDescriptorSet descriptorSet, VkBuffer uniformBuffer);

#endif