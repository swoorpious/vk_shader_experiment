// copyright 2025 swaroop.

#ifndef VK_SHADER_EXP_VIEWPORT_H
#define VK_SHADER_EXP_VIEWPORT_H

#include <util/math.h>
#include <vulkan/vulkan_core.h>


struct SDL_Window;

enum WindowState {
    Windowed,
    Fullscreen,
    WindowedFullscreen
};

class Viewport {
public:
    Viewport() = default;

    void init(SDL_Window* windowHandle);
    
    void onResize();

    VkViewport toVkViewport(bool flipY = false) const;
    VkRect2D toScissor() const;

    Math::Vector2f getSize() const; // usually the pixel size (backbuffer)
    Math::Vector2f getPosition() const;
    Math::Vector2f getDepthRange() const;
    Math::Vector2f getPixelSize() const;
    Math::Vector2f getLogicalSize() const;
    Math::Vector2f getContentScale() const;
    float getAspectRatio() const;

private:
    SDL_Window* window = nullptr;

    Math::Vector2f position = {0.0f, 0.0f};
    Math::Vector2f size = {800.0f, 600.0f}; // active viewport size
    Math::Vector2f depthRange = {0.0f, 1.0f};
    
    Math::Vector2f pixelSize = {800.0f, 600.0f};
    Math::Vector2f logicalSize = {800.0f, 600.0f};
    Math::Vector2f contentScale = {1.0f, 1.0f};
    
    WindowState state = Windowed;

    void updateFromWindow();
};

#endif // VK_SHADER_EXP_VIEWPORT_H