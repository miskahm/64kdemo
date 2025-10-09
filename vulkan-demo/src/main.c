#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

#include "vulkan_setup.h"
#include "shadertoy_compat.h"
#include "audio_synthesis.h"
#include "sync_system.h"

const char* validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    DemoApp* app = glfwGetWindowUserPointer(window);
    app->framebufferResized = true;
}

static void initWindow(DemoApp* app) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
    
    app->window = glfwCreateWindow(WIDTH, HEIGHT, APP_NAME, NULL, NULL);
    if (!app->window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwSetWindowUserPointer(app->window, app);
    glfwSetFramebufferSizeCallback(app->window, framebufferResizeCallback);
    glfwFocusWindow(app->window);
    glfwShowWindow(app->window);
    printf("Window created successfully\n");
}

static void mainLoop(DemoApp* app, AudioEngine* audio, RocketSync* sync);
static void cleanup(DemoApp* app);

int main() {
    printf("Starting 64K Vulkan Demo...\n");
    fflush(stdout);
    DemoApp app = {0};
    AudioEngine audio = {0};
    RocketSync sync = {0};
    
    printf("Initializing window...\n");
    fflush(stdout);
    initWindow(&app);
    
    printf("Initializing Vulkan...\n");
    fflush(stdout);
    initVulkan(&app);
    
    printf("Initializing audio...\n");
    fflush(stdout);
    audio_init(&audio, 44100.0f);
    
    if (audio_device_init(&audio) != 0) {
        fprintf(stderr, "Failed to initialize audio device\n");
        cleanup(&app);
        return 1;
    }
    
    audio_device_start(&audio);
    
    printf("Initializing sync system...\n");
    fflush(stdout);
    sync_init(&sync);
    
    printf("Entering main loop...\n");
    fflush(stdout);
    mainLoop(&app, &audio, &sync);
    
    printf("Cleaning up...\n");
    fflush(stdout);
    audio_device_cleanup(&audio);
    cleanup(&app);
    
    printf("Demo finished successfully\n");
    return 0;
}

static void mainLoop(DemoApp* app, AudioEngine* audio, RocketSync* sync) {
    double startTime = glfwGetTime();
    int frame = 0;
    double lastTime = 0.0;
    
    printf("Main loop started\n"); fflush(stdout);
    
    while (!glfwWindowShouldClose(app->window)) {
        glfwPollEvents();
        
        if (glfwWindowShouldClose(app->window)) {
            printf("Window close requested at frame %d\n", frame);
            break;
        }
        
        double currentTime = glfwGetTime() - startTime;
        float dt = (float)(currentTime - lastTime);
        lastTime = currentTime;
        
        audio_update(audio, dt);
        sync_update(sync, audio, dt);
        drawFrame(app, (float)currentTime, frame, audio, sync);
        
        frame++;
        
        if (frame % 600 == 0) {
            printf("Frame %d: Still running, time=%.2f\n", frame, currentTime);
            fflush(stdout);
        }
    }
    
    printf("Exiting main loop at frame %d\n", frame); fflush(stdout);
    vkDeviceWaitIdle(app->device);
}

static void cleanup(DemoApp* app) {
    cleanupVulkan(app);
    glfwDestroyWindow(app->window);
    glfwTerminate();
}