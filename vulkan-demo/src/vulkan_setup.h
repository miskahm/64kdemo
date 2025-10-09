#ifndef VULKAN_SETUP_H
#define VULKAN_SETUP_H

#include <stdbool.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#define APP_NAME "64K Demo"
#define WIDTH 1920
#define HEIGHT 1080
#define MAX_FRAMES_IN_FLIGHT 2

extern const char* validationLayers[];
extern const char* deviceExtensions[];

#ifdef NDEBUG
#define enableValidationLayers false
#else
#define enableValidationLayers true
#endif

typedef struct {
    GLFWwindow* window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSwapchainKHR swapChain;
    VkImage* swapChainImages;
    VkImageView* swapChainImageViews;
    uint32_t swapChainImageCount;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkFramebuffer* swapChainFramebuffers;
    VkCommandPool commandPool;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBufferMemory;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkCommandBuffer* commandBuffers;
    VkSemaphore* imageAvailableSemaphores;
    VkSemaphore* renderFinishedSemaphores;
    VkFence* inFlightFences;
    uint32_t currentFrame;
    bool framebufferResized;
} DemoApp;

void initVulkan(DemoApp* app);
void cleanupVulkan(DemoApp* app);
void createSurface(DemoApp* app);
void pickPhysicalDevice(DemoApp* app);
void createLogicalDevice(DemoApp* app);
void createSwapChain(DemoApp* app);
void createImageViews(DemoApp* app);
void createRenderPass(DemoApp* app);
void createGraphicsPipeline(DemoApp* app);
void createFramebuffers(DemoApp* app);
void createCommandPool(DemoApp* app);
void createVertexBuffer(DemoApp* app);
void createUniformBuffer(DemoApp* app);
void createDescriptorSetLayout(DemoApp* app);
void createDescriptorPool(DemoApp* app);
void createDescriptorSets(DemoApp* app);
void createCommandBuffers(DemoApp* app);
void createSyncObjects(DemoApp* app);
void drawFrame(DemoApp* app, float currentTime, int frame, void* audio, void* sync);
void recreateSwapChain(DemoApp* app);
void cleanupSwapChain(DemoApp* app);

#endif