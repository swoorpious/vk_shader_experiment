// copyright 2025 swaroop.

#ifndef VK_SHADER_EXP_ENGINE_H
#define VK_SHADER_EXP_ENGINE_H

struct SDL_Window;
struct VkInstance_T;
typedef struct VkInstance_T* VkInstance;
struct VkSurfaceKHR_T;
typedef struct VkSurfaceKHR_T* VkSurfaceKHR;

class Engine {
public:
    Engine();
    ~Engine();

    void run();

private:
    SDL_Window* window = nullptr;
    VkInstance instance = nullptr;
    VkSurfaceKHR surface = nullptr;

    void initSDL();
    void initWindow();
    void initVulkan();
};


#endif //VK_SHADER_EXP_ENGINE_H