// copyright 2025 swaroop.

#include "Engine.h"

#include <iostream>
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
    std::cout << "Shutdown complete.\n";
}

void Engine::run() {
    std::cout << "App running. Close window to exit.\n";
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
    }
}

void Engine::initSDL()  {
    // Enable standard logging to see internal SDL errors
    SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        // Detailed diagnostic if init fails
        const char* err = SDL_GetError();
        std::cerr << "SDL_Init failed: " << (err ? err : "Unknown error") << "\n";
            
        int numDrivers = SDL_GetNumVideoDrivers();
        std::cerr << "Available Video Drivers: " << numDrivers << "\n";
        for(int i=0; i < numDrivers; i++) {
            std::cerr << " - " << SDL_GetVideoDriver(i) << "\n";
        }
            
        if (numDrivers == 0) {
            std::cerr << "CRITICAL: SDL was built without any video backends! \n"
                      << "If on Linux, install X11/Wayland dev headers before running CMake.\n";
        }
        throw std::runtime_error("Failed to initialize SDL Video");
    }
}

void Engine::initWindow() {
    {
        window = SDL_CreateWindow(
            "sdl+vulkan engine test",
            800, 600,
            SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
        );

        if (!window) {
            throw std::runtime_error(std::string("failed to create window: ") + SDL_GetError());
        }
    }
}

void Engine::initVulkan() {
    // 1. Get Extensions
    Uint32 extCount = 0;
    const char* const* extNames = SDL_Vulkan_GetInstanceExtensions(&extCount);
    if (!extNames) {
        throw std::runtime_error("SDL could not get Vulkan extensions. Is Vulkan supported?");
    }
    std::vector<const char*> extensions(extNames, extNames + extCount);

    // 2. Create Instance
    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();

    // Optional: Enable validation layers here if you want debug output
        
    if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan Instance");
    }

    // 3. Create Surface
    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)) {
        throw std::runtime_error(std::string("Failed to create Vulkan Surface: ") + SDL_GetError());
    }
}
