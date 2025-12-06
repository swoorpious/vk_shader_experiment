// copyright 2025 swaroop.

#include <engine.h>

#include <cstdio>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <vector>
#include <stdexcept>

Engine::Engine() {
    initSDL();
    initWindow();
    initVulkan();
}

Engine::~Engine() {
    if (surface) {
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }
    if (instance) {
        vkDestroyInstance(instance, nullptr);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
    printf("shutdown complete\n");
}

void Engine::run() {
    printf("app running\n");
    bool running = true;
    
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            
            if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED || 
                event.type == SDL_EVENT_WINDOW_RESIZED) {
                viewport.onResize();
            }
        }

        // Render loop would use:
        // VkViewport vp = viewport.toVkViewport(true);
        // vkCmdSetViewport(cmd, 0, 1, &vp);
    }
}

void Engine::initSDL()  {
    SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        const char* err = SDL_GetError();
        fprintf(stderr, "SDL_Init failed: %s\n", (err ? err : "unknown error"));
        throw std::runtime_error("failed to initialize SDL Video");
    }
}

void Engine::initWindow() {
    window = SDL_CreateWindow(
        "sdl+vulkan Engine test",
        800, 600,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
    );

    if (!window) {
        throw std::runtime_error(std::string("failed to create window: ") + SDL_GetError());
    }

    viewport.init(window);
    
    printf("[viewport] initialized; logical: %fx%f pixel: %fx%f\n", 
           viewport.getLogicalSize().x, viewport.getLogicalSize().y,
           viewport.getPixelSize().x, viewport.getPixelSize().y);
}

void Engine::initVulkan() {
    Uint32 extCount = 0;
    const char* const* extNames = SDL_Vulkan_GetInstanceExtensions(&extCount);
    if (!extNames) {
        throw std::runtime_error("SDL could not get Vulkan extensions.");
    }
    std::vector<const char*> extensions(extNames, extNames + extCount);

    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();

    if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create Vulkan instance");
    }

    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)) {
        throw std::runtime_error(std::string("failed to create Vulkan Surface: ") + SDL_GetError());
    }
}