#include "vulkan_setup.h"
#include "shader_loader.h"
#include "shadertoy_compat.h"
#include "audio_synthesis.h"
#include "sync_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        fprintf(stderr, "Validation layer: %s\n", pCallbackData->pMessage);
    }
    return VK_FALSE;
}

static VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) {
    
    PFN_vkCreateDebugUtilsMessengerEXT func = 
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT");
    
    if (func != NULL) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator) {
    
    PFN_vkDestroyDebugUtilsMessengerEXT func = 
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkDestroyDebugUtilsMessengerEXT");
    
    if (func != NULL) {
        func(instance, debugMessenger, pAllocator);
    }
}

static bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    
    VkLayerProperties* availableLayers = malloc(layerCount * sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);
    
    for (int i = 0; i < 1; i++) {
        bool layerFound = false;
        for (uint32_t j = 0; j < layerCount; j++) {
            if (strcmp(validationLayers[i], availableLayers[j].layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            free(availableLayers);
            return false;
        }
    }
    
    free(availableLayers);
    return true;
}

static const char** getRequiredExtensions(uint32_t* extensionCount) {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    
    *extensionCount = glfwExtensionCount;
    if (enableValidationLayers) {
        (*extensionCount)++;
    }
    
    const char** extensions = malloc(*extensionCount * sizeof(char*));
    for (uint32_t i = 0; i < glfwExtensionCount; i++) {
        extensions[i] = glfwExtensions[i];
    }
    
    if (enableValidationLayers) {
        extensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }
    
    return extensions;
}

static void createInstance(DemoApp* app) {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        fprintf(stderr, "Validation layers requested, but not available!\n");
        exit(EXIT_FAILURE);
    }
    
    VkApplicationInfo appInfo = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = APP_NAME;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    
    VkInstanceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    
    uint32_t extensionCount = 0;
    const char** extensions = getRequiredExtensions(&extensionCount);
    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = extensions;
    
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = validationLayers;
    } else {
        createInfo.enabledLayerCount = 0;
    }
    
    if (vkCreateInstance(&createInfo, NULL, &app->instance) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create instance!\n");
        exit(EXIT_FAILURE);
    }
    
    free(extensions);
}

static void setupDebugMessenger(DemoApp* app) {
    if (!enableValidationLayers) return;
    
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    
    if (CreateDebugUtilsMessengerEXT(app->instance, &createInfo, NULL, &app->debugMessenger) != VK_SUCCESS) {
        fprintf(stderr, "Failed to set up debug messenger!\n");
    }
}

void initVulkan(DemoApp* app) {
    printf("  - Creating Vulkan instance...\n"); fflush(stdout);
    createInstance(app);
    printf("  - Setting up debug messenger...\n"); fflush(stdout);
    setupDebugMessenger(app);
    printf("  - Creating surface...\n"); fflush(stdout);
    createSurface(app);
    printf("  - Picking physical device...\n"); fflush(stdout);
    pickPhysicalDevice(app);
    printf("  - Creating logical device...\n"); fflush(stdout);
    createLogicalDevice(app);
    printf("  - Creating swapchain...\n"); fflush(stdout);
    createSwapChain(app);
    printf("  - Creating image views...\n"); fflush(stdout);
    createImageViews(app);
    printf("  - Creating render pass...\n"); fflush(stdout);
    createRenderPass(app);
    printf("  - Creating descriptor set layout...\n"); fflush(stdout);
    createDescriptorSetLayout(app);
    printf("  - Creating graphics pipeline...\n"); fflush(stdout);
    createGraphicsPipeline(app);
    printf("  - Creating framebuffers...\n"); fflush(stdout);
    createFramebuffers(app);
    printf("  - Creating command pool...\n"); fflush(stdout);
    createCommandPool(app);
    printf("  - Creating vertex buffer...\n"); fflush(stdout);
    createVertexBuffer(app);
    printf("  - Creating uniform buffer...\n"); fflush(stdout);
    createUniformBuffer(app);
    printf("  - Creating descriptor pool...\n"); fflush(stdout);
    createDescriptorPool(app);
    printf("  - Creating descriptor sets...\n"); fflush(stdout);
    createDescriptorSets(app);
    printf("  - Creating command buffers...\n"); fflush(stdout);
    createCommandBuffers(app);
    printf("  - Creating sync objects...\n"); fflush(stdout);
    createSyncObjects(app);
    printf("Vulkan initialization complete!\n"); fflush(stdout);
}

void cleanupVulkan(DemoApp* app) {
    cleanupSwapChain(app);
    
    vkDestroyDescriptorPool(app->device, app->descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(app->device, app->descriptorSetLayout, NULL);
    
    vkDestroyBuffer(app->device, app->uniformBuffer, NULL);
    vkFreeMemory(app->device, app->uniformBufferMemory, NULL);
    
    vkDestroyBuffer(app->device, app->vertexBuffer, NULL);
    vkFreeMemory(app->device, app->vertexBufferMemory, NULL);
    
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(app->device, app->imageAvailableSemaphores[i], NULL);
        vkDestroyFence(app->device, app->inFlightFences[i], NULL);
    }
    for (size_t i = 0; i < app->swapChainImageCount; i++) {
        vkDestroySemaphore(app->device, app->renderFinishedSemaphores[i], NULL);
    }
    
    vkDestroyCommandPool(app->device, app->commandPool, NULL);
    vkDestroyDevice(app->device, NULL);
    
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(app->instance, app->debugMessenger, NULL);
    }
    
    vkDestroySurfaceKHR(app->instance, app->surface, NULL);
    vkDestroyInstance(app->instance, NULL);
}

void createSurface(DemoApp* app) {
    if (glfwCreateWindowSurface(app->instance, app->window, NULL, &app->surface) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create window surface!\n");
        exit(EXIT_FAILURE);
    }
}

struct QueueFamilyIndices {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool graphicsFamilyFound;
    bool presentFamilyFound;
};

static struct QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    struct QueueFamilyIndices indices = {0};
    
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
    
    VkQueueFamilyProperties* queueFamilies = malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);
    
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            indices.graphicsFamilyFound = true;
        }
        
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        
        if (presentSupport) {
            indices.presentFamily = i;
            indices.presentFamilyFound = true;
        }
        
        if (indices.graphicsFamilyFound && indices.presentFamilyFound) {
            break;
        }
    }
    
    free(queueFamilies);
    return indices;
}

static bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    struct QueueFamilyIndices indices = findQueueFamilies(device, surface);
    
    if (!indices.graphicsFamilyFound || !indices.presentFamilyFound) {
        return false;
    }
    
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);
    
    VkExtensionProperties* availableExtensions = malloc(extensionCount * sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);
    
    bool swapChainAdequate = false;
    for (uint32_t i = 0; i < extensionCount; i++) {
        if (strcmp(availableExtensions[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
            swapChainAdequate = true;
            break;
        }
    }
    
    free(availableExtensions);
    
    return swapChainAdequate;
}

void pickPhysicalDevice(DemoApp* app) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(app->instance, &deviceCount, NULL);
    
    if (deviceCount == 0) {
        fprintf(stderr, "Failed to find GPUs with Vulkan support!\n");
        exit(EXIT_FAILURE);
    }
    
    VkPhysicalDevice* devices = malloc(deviceCount * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(app->instance, &deviceCount, devices);
    
    for (uint32_t i = 0; i < deviceCount; i++) {
        if (isDeviceSuitable(devices[i], app->surface)) {
            app->physicalDevice = devices[i];
            break;
        }
    }
    
    if (app->physicalDevice == VK_NULL_HANDLE) {
        fprintf(stderr, "Failed to find a suitable GPU!\n");
        exit(EXIT_FAILURE);
    }
    
    free(devices);
}

void createLogicalDevice(DemoApp* app) {
    struct QueueFamilyIndices indices = findQueueFamilies(app->physicalDevice, app->surface);
    
    VkDeviceQueueCreateInfo queueCreateInfos[2];
    float queuePriority = 1.0f;
    
    queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[0].queueFamilyIndex = indices.graphicsFamily;
    queueCreateInfos[0].queueCount = 1;
    queueCreateInfos[0].pQueuePriorities = &queuePriority;
    queueCreateInfos[0].pNext = NULL;
    queueCreateInfos[0].flags = 0;
    
    queueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[1].queueFamilyIndex = indices.presentFamily;
    queueCreateInfos[1].queueCount = 1;
    queueCreateInfos[1].pQueuePriorities = &queuePriority;
    queueCreateInfos[1].pNext = NULL;
    queueCreateInfos[1].flags = 0;
    
    VkPhysicalDeviceFeatures deviceFeatures = {0};
    
    VkDeviceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = (indices.graphicsFamily == indices.presentFamily) ? 1 : 2;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 1;
    createInfo.ppEnabledExtensionNames = deviceExtensions;
    
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = validationLayers;
    } else {
        createInfo.enabledLayerCount = 0;
    }
    
    if (vkCreateDevice(app->physicalDevice, &createInfo, NULL, &app->device) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create logical device!\n");
        exit(EXIT_FAILURE);
    }
    
    vkGetDeviceQueue(app->device, indices.graphicsFamily, 0, &app->graphicsQueue);
    vkGetDeviceQueue(app->device, indices.presentFamily, 0, &app->presentQueue);
}

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* formats;
    uint32_t formatCount;
    VkPresentModeKHR* presentModes;
    uint32_t presentModeCount;
};

static struct SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    struct SwapChainSupportDetails details = {0};
    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
    
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatCount, NULL);
    if (details.formatCount != 0) {
        details.formats = malloc(details.formatCount * sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatCount, details.formats);
    }
    
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.presentModeCount, NULL);
    if (details.presentModeCount != 0) {
        details.presentModes = malloc(details.presentModeCount * sizeof(VkPresentModeKHR));
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.presentModeCount, details.presentModes);
    }
    
    return details;
}

static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* availableFormats, uint32_t formatCount) {
    for (uint32_t i = 0; i < formatCount; i++) {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && 
            availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormats[i];
        }
    }
    return availableFormats[0];
}

static VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR* availablePresentModes, uint32_t presentModeCount) {
    for (uint32_t i = 0; i < presentModeCount; i++) {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentModes[i];
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR* capabilities) {
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    } else {
        VkExtent2D actualExtent = {WIDTH, HEIGHT};
        
        actualExtent.width = (actualExtent.width > capabilities->maxImageExtent.width) ? 
                            capabilities->maxImageExtent.width : actualExtent.width;
        actualExtent.width = (actualExtent.width < capabilities->minImageExtent.width) ? 
                            capabilities->minImageExtent.width : actualExtent.width;
                            
        actualExtent.height = (actualExtent.height > capabilities->maxImageExtent.height) ? 
                             capabilities->maxImageExtent.height : actualExtent.height;
        actualExtent.height = (actualExtent.height < capabilities->minImageExtent.height) ? 
                             capabilities->minImageExtent.height : actualExtent.height;
                             
        return actualExtent;
    }
}

void createSwapChain(DemoApp* app) {
    struct SwapChainSupportDetails swapChainSupport = querySwapChainSupport(app->physicalDevice, app->surface);
    
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats, swapChainSupport.formatCount);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, swapChainSupport.presentModeCount);
    VkExtent2D extent = chooseSwapExtent(&swapChainSupport.capabilities);
    
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    
    VkSwapchainCreateInfoKHR createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = app->surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    struct QueueFamilyIndices indices = findQueueFamilies(app->physicalDevice, app->surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};
    
    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    
    if (vkCreateSwapchainKHR(app->device, &createInfo, NULL, &app->swapChain) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create swap chain!\n");
        exit(EXIT_FAILURE);
    }
    
    vkGetSwapchainImagesKHR(app->device, app->swapChain, &imageCount, NULL);
    app->swapChainImages = malloc(imageCount * sizeof(VkImage));
    vkGetSwapchainImagesKHR(app->device, app->swapChain, &imageCount, app->swapChainImages);
    
    app->swapChainImageCount = imageCount;
    app->swapChainImageFormat = surfaceFormat.format;
    app->swapChainExtent = extent;
    
    free(swapChainSupport.formats);
    free(swapChainSupport.presentModes);
}

void createImageViews(DemoApp* app) {
    uint32_t swapChainImageCount;
    vkGetSwapchainImagesKHR(app->device, app->swapChain, &swapChainImageCount, NULL);
    
    app->swapChainImageViews = malloc(swapChainImageCount * sizeof(VkImageView));
    
    for (uint32_t i = 0; i < swapChainImageCount; i++) {
        VkImageViewCreateInfo createInfo = {0};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = app->swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = app->swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        
        if (vkCreateImageView(app->device, &createInfo, NULL, &app->swapChainImageViews[i]) != VK_SUCCESS) {
            fprintf(stderr, "Failed to create image views!\n");
            exit(EXIT_FAILURE);
        }
    }
}

void createRenderPass(DemoApp* app) {
    VkAttachmentDescription colorAttachment = {0};
    colorAttachment.format = app->swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentReference colorAttachmentRef = {0};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    
    VkSubpassDependency dependency = {0};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
    VkRenderPassCreateInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    
    if (vkCreateRenderPass(app->device, &renderPassInfo, NULL, &app->renderPass) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create render pass!\n");
        exit(EXIT_FAILURE);
    }
}

void createDescriptorSetLayout(DemoApp* app) {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {0};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    uboLayoutBinding.pImmutableSamplers = NULL;
    
    VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;
    
    if (vkCreateDescriptorSetLayout(app->device, &layoutInfo, NULL, &app->descriptorSetLayout) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create descriptor set layout!\n");
        exit(EXIT_FAILURE);
    }
}

void createGraphicsPipeline(DemoApp* app) {
    printf("  - Loading shaders...\n"); fflush(stdout);
    VkShaderModule vertShaderModule = createShaderModule(app->device, "shader.vert.spv");
    VkShaderModule fragShaderModule = createShaderModule(app->device, "shader.frag.spv");
    
    if (vertShaderModule == VK_NULL_HANDLE || fragShaderModule == VK_NULL_HANDLE) {
        fprintf(stderr, "FATAL: Failed to load shader modules!\n");
        fprintf(stderr, "  Vertex shader: %s\n", vertShaderModule == VK_NULL_HANDLE ? "FAILED" : "OK");
        fprintf(stderr, "  Fragment shader: %s\n", fragShaderModule == VK_NULL_HANDLE ? "FAILED" : "OK");
        if (vertShaderModule != VK_NULL_HANDLE) vkDestroyShaderModule(app->device, vertShaderModule, NULL);
        if (fragShaderModule != VK_NULL_HANDLE) vkDestroyShaderModule(app->device, fragShaderModule, NULL);
        exit(EXIT_FAILURE);
    }
    printf("  - Shaders loaded successfully\n"); fflush(stdout);
    
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {0};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {0};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
    
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {0};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    
    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)app->swapChainExtent.width;
    viewport.height = (float)app->swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    VkRect2D scissor = {0};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = app->swapChainExtent;
    
    VkPipelineViewportStateCreateInfo viewportState = {0};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
    
    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    
    VkPipelineMultisampleStateCreateInfo multisampling = {0};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    
    VkPipelineColorBlendStateCreateInfo colorBlending = {0};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &app->descriptorSetLayout;
    
    if (vkCreatePipelineLayout(app->device, &pipelineLayoutInfo, NULL, &app->pipelineLayout) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create pipeline layout!\n");
        exit(EXIT_FAILURE);
    }
    
    VkGraphicsPipelineCreateInfo pipelineInfo = {0};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = app->pipelineLayout;
    pipelineInfo.renderPass = app->renderPass;
    pipelineInfo.subpass = 0;
    
    if (vkCreateGraphicsPipelines(app->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &app->graphicsPipeline) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create graphics pipeline!\n");
        exit(EXIT_FAILURE);
    }
    
    vkDestroyShaderModule(app->device, fragShaderModule, NULL);
    vkDestroyShaderModule(app->device, vertShaderModule, NULL);
}

void createFramebuffers(DemoApp* app) {
    uint32_t swapChainImageCount;
    vkGetSwapchainImagesKHR(app->device, app->swapChain, &swapChainImageCount, NULL);
    
    app->swapChainFramebuffers = malloc(swapChainImageCount * sizeof(VkFramebuffer));
    
    for (uint32_t i = 0; i < swapChainImageCount; i++) {
        VkImageView attachments[] = {app->swapChainImageViews[i]};
        
        VkFramebufferCreateInfo framebufferInfo = {0};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = app->renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = app->swapChainExtent.width;
        framebufferInfo.height = app->swapChainExtent.height;
        framebufferInfo.layers = 1;
        
        if (vkCreateFramebuffer(app->device, &framebufferInfo, NULL, &app->swapChainFramebuffers[i]) != VK_SUCCESS) {
            fprintf(stderr, "Failed to create framebuffer!\n");
            exit(EXIT_FAILURE);
        }
    }
}

void createCommandPool(DemoApp* app) {
    struct QueueFamilyIndices queueFamilyIndices = findQueueFamilies(app->physicalDevice, app->surface);
    
    VkCommandPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
    
    if (vkCreateCommandPool(app->device, &poolInfo, NULL, &app->commandPool) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create command pool!\n");
        exit(EXIT_FAILURE);
    }
}

void createVertexBuffer(DemoApp* app) {
    VkBufferCreateInfo bufferInfo = {0};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(float) * 6;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(app->device, &bufferInfo, NULL, &app->vertexBuffer) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create vertex buffer!\n");
        exit(EXIT_FAILURE);
    }
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(app->device, app->vertexBuffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(app->physicalDevice, &memProperties);
    
    uint32_t memoryTypeIndex;
    for (memoryTypeIndex = 0; memoryTypeIndex < memProperties.memoryTypeCount; memoryTypeIndex++) {
        if ((memProperties.memoryTypes[memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
            (memProperties.memoryTypes[memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
            break;
        }
    }
    
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    
    if (vkAllocateMemory(app->device, &allocInfo, NULL, &app->vertexBufferMemory) != VK_SUCCESS) {
        fprintf(stderr, "Failed to allocate vertex buffer memory!\n");
        exit(EXIT_FAILURE);
    }
    
    vkBindBufferMemory(app->device, app->vertexBuffer, app->vertexBufferMemory, 0);
    
    void* data;
    vkMapMemory(app->device, app->vertexBufferMemory, 0, bufferInfo.size, 0, &data);
    float vertices[] = {-1.0f, -1.0f, 3.0f, -1.0f, -1.0f, 3.0f};
    memcpy(data, vertices, sizeof(vertices));
    vkUnmapMemory(app->device, app->vertexBufferMemory);
}

void createUniformBuffer(DemoApp* app) {
    VkDeviceSize bufferSize = 128;
    
    VkBufferCreateInfo bufferInfo = {0};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(app->device, &bufferInfo, NULL, &app->uniformBuffer) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create uniform buffer!\n");
        exit(EXIT_FAILURE);
    }
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(app->device, app->uniformBuffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(app->physicalDevice, &memProperties);
    
    uint32_t memoryTypeIndex;
    for (memoryTypeIndex = 0; memoryTypeIndex < memProperties.memoryTypeCount; memoryTypeIndex++) {
        if ((memProperties.memoryTypes[memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
            (memProperties.memoryTypes[memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
            break;
        }
    }
    
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    
    if (vkAllocateMemory(app->device, &allocInfo, NULL, &app->uniformBufferMemory) != VK_SUCCESS) {
        fprintf(stderr, "Failed to allocate uniform buffer memory!\n");
        exit(EXIT_FAILURE);
    }
    
    vkBindBufferMemory(app->device, app->uniformBuffer, app->uniformBufferMemory, 0);
}

void createDescriptorPool(DemoApp* app) {
    VkDescriptorPoolSize poolSize = {0};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 1;
    
    VkDescriptorPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;
    
    if (vkCreateDescriptorPool(app->device, &poolInfo, NULL, &app->descriptorPool) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create descriptor pool!\n");
        exit(EXIT_FAILURE);
    }
}

void createDescriptorSets(DemoApp* app) {
    VkDescriptorSetLayout layouts[] = {app->descriptorSetLayout};
    
    VkDescriptorSetAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = app->descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts;
    
    if (vkAllocateDescriptorSets(app->device, &allocInfo, &app->descriptorSet) != VK_SUCCESS) {
        fprintf(stderr, "Failed to allocate descriptor sets!\n");
        exit(EXIT_FAILURE);
    }
    
    VkDescriptorBufferInfo bufferInfo = {0};
    bufferInfo.buffer = app->uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = 128;
    
    VkWriteDescriptorSet descriptorWrite = {0};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = app->descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    
    vkUpdateDescriptorSets(app->device, 1, &descriptorWrite, 0, NULL);
}

void createCommandBuffers(DemoApp* app) {
    uint32_t swapChainImageCount;
    vkGetSwapchainImagesKHR(app->device, app->swapChain, &swapChainImageCount, NULL);
    
    app->commandBuffers = malloc(swapChainImageCount * sizeof(VkCommandBuffer));
    
    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = app->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = swapChainImageCount;
    
    if (vkAllocateCommandBuffers(app->device, &allocInfo, app->commandBuffers) != VK_SUCCESS) {
        fprintf(stderr, "Failed to allocate command buffers!\n");
        exit(EXIT_FAILURE);
    }
    
    for (uint32_t i = 0; i < swapChainImageCount; i++) {
        VkCommandBufferBeginInfo beginInfo = {0};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        
        if (vkBeginCommandBuffer(app->commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            fprintf(stderr, "Failed to begin recording command buffer!\n");
            exit(EXIT_FAILURE);
        }
        
        VkRenderPassBeginInfo renderPassInfo = {0};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = app->renderPass;
        renderPassInfo.framebuffer = app->swapChainFramebuffers[i];
        renderPassInfo.renderArea.offset.x = 0;
        renderPassInfo.renderArea.offset.y = 0;
        renderPassInfo.renderArea.extent = app->swapChainExtent;
        
        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;
        
        vkCmdBeginRenderPass(app->commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(app->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, app->graphicsPipeline);
        vkCmdBindDescriptorSets(app->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, app->pipelineLayout, 0, 1, &app->descriptorSet, 0, NULL);
        vkCmdDraw(app->commandBuffers[i], 3, 1, 0, 0);
        vkCmdEndRenderPass(app->commandBuffers[i]);
        
        if (vkEndCommandBuffer(app->commandBuffers[i]) != VK_SUCCESS) {
            fprintf(stderr, "Failed to record command buffer!\n");
            exit(EXIT_FAILURE);
        }
    }
}

void createSyncObjects(DemoApp* app) {
    app->imageAvailableSemaphores = malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkSemaphore));
    app->renderFinishedSemaphores = malloc(app->swapChainImageCount * sizeof(VkSemaphore));
    app->inFlightFences = malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkFence));
    
    VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(app->device, &semaphoreInfo, NULL, &app->imageAvailableSemaphores[i]) != VK_SUCCESS) {
            fprintf(stderr, "Failed to create imageAvailable semaphores!\n");
            exit(EXIT_FAILURE);
        }
        if (vkCreateFence(app->device, &fenceInfo, NULL, &app->inFlightFences[i]) != VK_SUCCESS) {
            fprintf(stderr, "Failed to create fences!\n");
            exit(EXIT_FAILURE);
        }
    }
    
    for (size_t i = 0; i < app->swapChainImageCount; i++) {
        if (vkCreateSemaphore(app->device, &semaphoreInfo, NULL, &app->renderFinishedSemaphores[i]) != VK_SUCCESS) {
            fprintf(stderr, "Failed to create renderFinished semaphores!\n");
            exit(EXIT_FAILURE);
        }
    }
}

void drawFrame(DemoApp* app, float currentTime, int frame, void* audio, void* sync) {
    if (!app || !app->device || !app->window) {
        fprintf(stderr, "ERROR: Invalid app state in drawFrame (frame %d)\n", frame);
        return;
    }
    
    VkResult fenceResult = vkWaitForFences(app->device, 1, &app->inFlightFences[app->currentFrame], VK_TRUE, UINT64_MAX);
    if (fenceResult != VK_SUCCESS) {
        fprintf(stderr, "ERROR: vkWaitForFences failed with code %d (frame %d)\n", fenceResult, frame);
        return;
    }
    
    uint32_t imageIndex;
    VkSemaphore imageAvailableSemaphore = app->imageAvailableSemaphores[app->currentFrame];
    VkResult result = vkAcquireNextImageKHR(app->device, app->swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        fprintf(stderr, "DEBUG: Swapchain out of date, recreating... (frame %d)\n", frame);
        fflush(stderr);
        recreateSwapChain(app);
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        fprintf(stderr, "ERROR: Failed to acquire swap chain image! Result code: %d (frame %d)\n", result, frame);
        fflush(stderr);
        return;
    }
    
    if (frame < 5 || frame % 60 == 0) {
        printf("Frame %d: imageIndex=%u currentFrame=%u time=%.2f\n", frame, imageIndex, app->currentFrame, currentTime);
        fflush(stdout);
    }
    
    vkResetFences(app->device, 1, &app->inFlightFences[app->currentFrame]);
    
    if (frame % 300 == 0) {
        printf("About to update uniforms (frame %d)...\n", frame);
        fflush(stdout);
    }
    
    updateUniforms(app, currentTime, frame, (AudioEngine*)audio, (RocketSync*)sync);
    
    if (frame % 300 == 0) {
        printf("Uniforms updated successfully (frame %d)\n", frame);
        fflush(stdout);
    }
    
    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &app->commandBuffers[imageIndex];
    
    VkSemaphore signalSemaphores[] = {app->renderFinishedSemaphores[imageIndex]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    VkResult submitResult = vkQueueSubmit(app->graphicsQueue, 1, &submitInfo, app->inFlightFences[app->currentFrame]);
    if (submitResult != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to submit draw command buffer! Result code: %d (frame %d)\n", submitResult, frame);
        exit(EXIT_FAILURE);
    }
    
    VkPresentInfoKHR presentInfo = {0};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    VkSwapchainKHR swapChains[] = {app->swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    
    result = vkQueuePresentKHR(app->presentQueue, &presentInfo);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || app->framebufferResized) {
        fprintf(stderr, "DEBUG: Swapchain needs recreation (present), result=%d resized=%d (frame %d)\n", result, app->framebufferResized, frame);
        app->framebufferResized = false;
        recreateSwapChain(app);
    } else if (result != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to present swap chain image! Result code: %d (frame %d)\n", result, frame);
    }
    
    app->currentFrame = (app->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void cleanupSwapChain(DemoApp* app) {
    uint32_t swapChainImageCount;
    vkGetSwapchainImagesKHR(app->device, app->swapChain, &swapChainImageCount, NULL);
    
    for (uint32_t i = 0; i < swapChainImageCount; i++) {
        vkDestroyFramebuffer(app->device, app->swapChainFramebuffers[i], NULL);
    }
    free(app->swapChainFramebuffers);
    
    free(app->commandBuffers);
    
    vkDestroyPipeline(app->device, app->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(app->device, app->pipelineLayout, NULL);
    vkDestroyRenderPass(app->device, app->renderPass, NULL);
    
    for (uint32_t i = 0; i < swapChainImageCount; i++) {
        vkDestroyImageView(app->device, app->swapChainImageViews[i], NULL);
    }
    free(app->swapChainImageViews);
    
    vkDestroySwapchainKHR(app->device, app->swapChain, NULL);
}

void recreateSwapChain(DemoApp* app) {
    int width = 0, height = 0;
    glfwGetFramebufferSize(app->window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(app->window, &width, &height);
        glfwWaitEvents();
    }
    
    vkDeviceWaitIdle(app->device);
    
    cleanupSwapChain(app);
    
    createSwapChain(app);
    createImageViews(app);
    createRenderPass(app);
    createGraphicsPipeline(app);
    createFramebuffers(app);
    createCommandBuffers(app);
}