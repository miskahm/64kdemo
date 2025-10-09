#include "shadertoy_compat.h"
#include <string.h>
#include <stdio.h>

typedef struct {
    float iTime;
    float _padding1;
    float iResolution[2];
    float iMouse[4];
    int iFrame;
    int iScene;
    float iTransition;
    float iBass;
    float iMid;
    float iHigh;
    float iIntensity;
    int iKick;
    int iSnare;
    float _padding2;
} ShaderToyUniforms;

void updateUniforms(DemoApp* app, float currentTime, int frame, AudioEngine* audio, RocketSync* sync) {
    if (!app || !app->window || !sync) {
        fprintf(stderr, "ERROR: NULL pointer in updateUniforms (app=%p, window=%p, sync=%p)\n", 
                (void*)app, app ? (void*)app->window : NULL, (void*)sync);
        return;
    }
    
    ShaderToyUniforms uniforms = {0};
    
    uniforms.iTime = currentTime;
    uniforms.iResolution[0] = (float)app->swapChainExtent.width;
    uniforms.iResolution[1] = (float)app->swapChainExtent.height;
    uniforms.iFrame = frame;
    
    uniforms.iMouse[0] = 0.0f;
    uniforms.iMouse[1] = 0.0f;
    uniforms.iMouse[2] = 0.0f;
    uniforms.iMouse[3] = 0.0f;
    
    int sceneId = (int)(currentTime / 12.0) % 5;
    uniforms.iScene = sceneId;
    
    float sceneTime = currentTime - (sceneId * 12.0f);
    uniforms.iTransition = sceneTime < 1.0f ? sceneTime : (sceneTime > 11.0f ? (12.0f - sceneTime) : 1.0f);
    
    uniforms.iBass = sync->current.bass;
    uniforms.iMid = sync->current.mid;
    uniforms.iHigh = sync->current.high;
    uniforms.iIntensity = sync->current.intensity;
    uniforms.iKick = sync->current.kick ? 1 : 0;
    uniforms.iSnare = sync->current.snare ? 1 : 0;
    
    void* data;
    vkMapMemory(app->device, app->uniformBufferMemory, 0, sizeof(uniforms), 0, &data);
    memcpy(data, &uniforms, sizeof(uniforms));
    vkUnmapMemory(app->device, app->uniformBufferMemory);
}

VkDescriptorSetLayoutBinding createUniformBinding() {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {0};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    uboLayoutBinding.pImmutableSamplers = NULL;
    
    return uboLayoutBinding;
}

VkWriteDescriptorSet createUniformWrite(VkDescriptorSet descriptorSet, VkBuffer uniformBuffer) {
    VkDescriptorBufferInfo bufferInfo = {0};
    bufferInfo.buffer = uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(ShaderToyUniforms);
    
    VkWriteDescriptorSet descriptorWrite = {0};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    
    return descriptorWrite;
}